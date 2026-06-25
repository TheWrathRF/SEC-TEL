import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

public class Crypto {
    // same 16-byte key as the C sender
    private static final byte[] KEY = "sectelsecretkey1".getBytes();

    // decrypt len bytes of data (AES-128 ECB, PKCS5/PKCS7 padding)
    public static byte[] decrypt(byte[] data, int len) throws Exception {
        Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
        cipher.init(Cipher.DECRYPT_MODE, new SecretKeySpec(KEY, "AES"));
        return cipher.doFinal(data, 0, len);
    }
}
