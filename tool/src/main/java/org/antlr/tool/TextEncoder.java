package org.antlr.tool;

/**
 * Created by npohilets on 06.05.15.
 */
public abstract class TextEncoder {
    public static final String UTF8_ENCODING = "UTF8";
    public static final String UTF16_ENCODING = "UTF16";
    public static final String UTF32_ENCODING = "UTF32";
    public static final String DEFAULT_ENCODING = UTF16_ENCODING;

    public static TextEncoder getEncoder(String encoding) {
        if (encoding == null)
            return getEncoder(DEFAULT_ENCODING);
        if (encoding.equals(UTF8_ENCODING))
            return new UTF8TextEncoder();
        if (encoding.equals(UTF16_ENCODING))
            return new UTF16TextEncoder();
        if (encoding.equals(UTF32_ENCODING))
            return new UTF32TextEncoder();

        // TODO: Report error about invalid encoding
        return getEncoder(DEFAULT_ENCODING);
    }

    public abstract String getEncoding();
    public abstract int getMaxCodeValue();
    public abstract boolean isSingleCode(CharSequence s);
    public abstract int[] getCodes(CharSequence s);
}
