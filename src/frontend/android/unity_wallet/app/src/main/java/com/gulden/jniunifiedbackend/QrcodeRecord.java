// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

public final class QrcodeRecord {


    /*package*/ final int mWidth;

    /*package*/ final byte[] mPixelData;

    public QrcodeRecord(
            int width,
            byte[] pixelData) {
        this.mWidth = width;
        this.mPixelData = pixelData;
    }

    public int getWidth() {
        return mWidth;
    }

    public byte[] getPixelData() {
        return mPixelData;
    }

    @Override
    public String toString() {
        return "QrcodeRecord{" +
                "mWidth=" + mWidth +
                "," + "mPixelData=" + mPixelData +
        "}";
    }

}
