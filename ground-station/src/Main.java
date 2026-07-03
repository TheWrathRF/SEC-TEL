import java.net.BindException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketTimeoutException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.zip.CRC32;

public class Main {
    static final int CRC_LEN = 4;

    static ConnectionPool pool;
    static ExecutorService writers;

    public static void main(String[] args) throws Exception {
        pool = new ConnectionPool(4);
        writers = Executors.newFixedThreadPool(4);
        System.out.println("ground station up, following the port hops...");

        DatagramSocket sock = null;
        int curPort = -1;
        byte[] buf = new byte[1024];

        while (true) {
            long slot = (System.currentTimeMillis() / 1000) / Ports.HOP_SECONDS;
            int port = Ports.hopPort(slot);

            if (sock == null || port != curPort) {
                if (sock != null) sock.close();
                try {
                    sock = new DatagramSocket(port);
                    sock.setSoTimeout(1000);
                    curPort = port;
                    System.out.println("listening on port " + port);
                } catch (BindException e) {
                    System.out.println("could not bind " + port + ", skipping");
                    sock = null;
                    Thread.sleep(500);
                    continue;
                }
            }

            DatagramPacket pkt = new DatagramPacket(buf, buf.length);
            try {
                sock.receive(pkt);
            } catch (SocketTimeoutException e) {
                continue;
            }

            handlePacket(pkt);
        }
    }

    static void handlePacket(DatagramPacket pkt) throws Exception {
        int len = pkt.getLength();
        byte[] data = pkt.getData();
        String from = pkt.getAddress().getHostAddress();
        int srcPort = pkt.getPort();

        if (len <= CRC_LEN) {
            writers.submit(() -> logAttack(from, srcPort, len, "bad length"));
            return;
        }

        int payloadLen = len - CRC_LEN;
        CRC32 crc = new CRC32();
        crc.update(data, 0, payloadLen);
        long calc = crc.getValue();

        long recv = (data[payloadLen] & 0xFFL)
                  | ((data[payloadLen + 1] & 0xFFL) << 8)
                  | ((data[payloadLen + 2] & 0xFFL) << 16)
                  | ((data[payloadLen + 3] & 0xFFL) << 24);

        if (calc != recv) {
            writers.submit(() -> logAttack(from, srcPort, len, "crc mismatch"));
            System.out.println("CRC FAIL (" + len + " bytes from " + from + ") -> logged");
        } else {
            // decrypt on the receive thread, hand the insert to a writer
            byte[] plain = Crypto.decrypt(data, payloadLen);
            Telemetry t = Telemetry.parse(plain);
            writers.submit(() -> saveTelemetry(t));
            System.out.printf("OK  ts=%d alt=%.1f spd=%.1f batt=%.1f -> stored%n",
                              t.ts, t.altitude, t.speed, t.battery);
        }
    }

    static void saveTelemetry(Telemetry t) {
        Connection db = null;
        try {
            db = pool.borrow();
            String sql = "INSERT INTO clean_telemetry (ts, latitude, longitude, altitude, speed, battery) "
                       + "VALUES (?, ?, ?, ?, ?, ?)";
            try (PreparedStatement ps = db.prepareStatement(sql)) {
                ps.setLong(1, t.ts);
                ps.setDouble(2, t.latitude);
                ps.setDouble(3, t.longitude);
                ps.setFloat(4, t.altitude);
                ps.setFloat(5, t.speed);
                ps.setFloat(6, t.battery);
                ps.executeUpdate();
            }
        } catch (Exception e) {
            System.out.println("insert failed: " + e.getMessage());
        } finally {
            if (db != null) pool.release(db);
        }
    }

    static void logAttack(String addr, int srcPort, int size, String reason) {
        Connection db = null;
        try {
            db = pool.borrow();
            String sql = "INSERT INTO attack_logs (source_addr, source_port, packet_size, reason) "
                       + "VALUES (?, ?, ?, ?)";
            try (PreparedStatement ps = db.prepareStatement(sql)) {
                ps.setString(1, addr);
                ps.setInt(2, srcPort);
                ps.setInt(3, size);
                ps.setString(4, reason);
                ps.executeUpdate();
            }
        } catch (Exception e) {
            System.out.println("insert failed: " + e.getMessage());
        } finally {
            if (db != null) pool.release(db);
        }
    }
}
