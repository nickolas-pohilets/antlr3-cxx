package org.antlr.tool;

import java.io.UnsupportedEncodingException;

/**
 * Created by npohilets on 06.05.15.
 */
public class UTF32TextEncoder extends TextEncoder {
    @Override
    public String getEncoding() {
        return UTF32_ENCODING;
    }

    @Override
    public int getMaxCodeValue() {
        return 0x10FFFF;
    }

    @Override
    public boolean isSingleCode(CharSequence s) {
        if (s.length() == 1) return true;
        if (s.length() > 2) return false;
        return Character.isSurrogatePair(s.charAt(0), s.charAt(1));
    }

    @Override
    public int[] getCodes(CharSequence s) {
        try {
            byte[] bytes = s.toString().getBytes("UTF-32BE");
            assert (bytes.length % 4) == 0;
            int[] codes = new int[bytes.length / 4];
            for (int i = 0; i < bytes.length / 4; ++i) {
                assert bytes[4 * i] == 0;
                int code = ((bytes[4 * i + 1] * 256) + bytes[4 * i + 2] * 256) + bytes[4 * i + 3];
                codes[i] = code;
            }
            return codes;
        }
        catch (UnsupportedEncodingException e) {
            return null;
        }
    }
}
