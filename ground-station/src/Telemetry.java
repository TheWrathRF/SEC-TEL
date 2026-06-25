import java.nio.ByteBuffer;
import java.nio.ByteOrder;

// mirrors the telemetry struct sent by the UAV (little-endian)
public class Telemetry {
    long ts;
    double latitude;
    double longitude;
    float altitude;
    float speed;
    float battery;

    static Telemetry parse(byte[] b) {
        ByteBuffer bb = ByteBuffer.wrap(b).order(ByteOrder.LITTLE_ENDIAN);
        Telemetry t = new Telemetry();
        t.ts        = bb.getLong();
        t.latitude  = bb.getDouble();
        t.longitude = bb.getDouble();
        t.altitude  = bb.getFloat();
        t.speed     = bb.getFloat();
        t.battery   = bb.getFloat();
        return t;
    }
}
