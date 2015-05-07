package org.antlr.tool;

import java.io.UnsupportedEncodingException;

/**
 * Created by npohilets on 06.05.15.
 */
public class UTF8TextEncoder extends TextEncoder {
    @Override
    public String getEncoding() {
        return UTF8_ENCODING;
    }

    @Override
    public int getMaxCodeValue() {
        return 0xFF;
    }

    @Override
    public boolean isSingleCode(CharSequence s) {
        return s.length() == 1 && s.charAt(0) <= getMaxCodeValue();
    }

    @Override
    public int[] getCodes(CharSequence s) {
        try {
            byte[] bytes = s.toString().getBytes("UTF-8");
            int[] codes = new int[bytes.length];
            for (int i = 0; i < bytes.length; ++i) {
                codes[i] = ((int) bytes[i]) & 0xFF;
            }
            return codes;
        }
        catch (UnsupportedEncodingException ex) {
            return null;
        }
    }
}
