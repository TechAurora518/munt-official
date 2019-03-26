// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

public final class OutputRecord implements android.os.Parcelable {


    /*package*/ final long mAmount;

    /*package*/ final String mAddress;

    /*package*/ final String mLabel;

    /*package*/ final boolean mIsMine;

    public OutputRecord(
            long amount,
            String address,
            String label,
            boolean isMine) {
        this.mAmount = amount;
        this.mAddress = address;
        this.mLabel = label;
        this.mIsMine = isMine;
    }

    public long getAmount() {
        return mAmount;
    }

    public String getAddress() {
        return mAddress;
    }

    public String getLabel() {
        return mLabel;
    }

    public boolean getIsMine() {
        return mIsMine;
    }

    @Override
    public String toString() {
        return "OutputRecord{" +
                "mAmount=" + mAmount +
                "," + "mAddress=" + mAddress +
                "," + "mLabel=" + mLabel +
                "," + "mIsMine=" + mIsMine +
        "}";
    }


    public static final android.os.Parcelable.Creator<OutputRecord> CREATOR
        = new android.os.Parcelable.Creator<OutputRecord>() {
        @Override
        public OutputRecord createFromParcel(android.os.Parcel in) {
            return new OutputRecord(in);
        }

        @Override
        public OutputRecord[] newArray(int size) {
            return new OutputRecord[size];
        }
    };

    public OutputRecord(android.os.Parcel in) {
        this.mAmount = in.readLong();
        this.mAddress = in.readString();
        this.mLabel = in.readString();
        this.mIsMine = in.readByte() != 0;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(android.os.Parcel out, int flags) {
        out.writeLong(this.mAmount);
        out.writeString(this.mAddress);
        out.writeString(this.mLabel);
        out.writeByte(this.mIsMine ? (byte)1 : 0);
    }

}
