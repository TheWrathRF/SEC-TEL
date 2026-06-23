public class Ports {
    public static final int PORT_BASE   = 5000;
    public static final int PORT_SPAN   = 101;   // ports 5000..5100
    public static final int HOP_SECONDS = 5;

    // same predefined seed as the C sender
    private static final int HOP_SEED = 0x5eed1234;

    // destination port for a 5-second time slot, must match the C side
    public static int hopPort(long slot) {
        int r = (int) slot ^ HOP_SEED;
        r = r * 1103515245 + 12345;
        return PORT_BASE + ((r >>> 16) % PORT_SPAN);
    }
}
