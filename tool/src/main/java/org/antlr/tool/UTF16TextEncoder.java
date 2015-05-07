package org.antlr.tool;

/**
 * Created by npohilets on 06.05.15.
 */
public class UTF16TextEncoder extends TextEncoder {
    @Override
    public String getEncoding() {
        return UTF16_ENCODING;
    }

    @Override
    public int getMaxCodeValue() {
        return 0xFFFF;
    }

    @Override
    public boolean isSingleCode(CharSequence s) {
        return s.length() == 1;
    }

    @Override
    public int[] getCodes(CharSequence s) {
        int[] codes = new int[s.length()];
        for (int i = 0; i < s.length(); ++i) {
            codes[i] = s.charAt(i);
        }
        return codes;
    }
}
