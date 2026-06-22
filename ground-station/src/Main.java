import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.Statement;

public class Main {
    public static void main(String[] args) throws Exception {
        try (Connection conn = Database.connect()) {
            System.out.println("connected to sectel_db");

            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery(
                "SELECT table_name FROM information_schema.tables " +
                "WHERE table_schema = 'public' ORDER BY table_name");
            System.out.println("tables:");
            while (rs.next()) {
                System.out.println("  " + rs.getString("table_name"));
            }
        }
    }
}
