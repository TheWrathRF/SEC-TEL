import java.sql.Connection;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

// a small fixed pool of database connections so rapid inserts reuse a
// connection instead of each one opening (and closing) its own
public class ConnectionPool {
    private final BlockingQueue<Connection> pool;

    public ConnectionPool(int size) throws Exception {
        pool = new ArrayBlockingQueue<>(size);
        for (int i = 0; i < size; i++) {
            pool.add(Database.connect());
        }
    }

    public Connection borrow() throws InterruptedException {
        return pool.take();
    }

    public void release(Connection c) {
        pool.offer(c);
    }
}
