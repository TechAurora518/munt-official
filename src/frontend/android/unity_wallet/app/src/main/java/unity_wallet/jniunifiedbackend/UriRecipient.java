// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package unity_wallet.jniunifiedbackend;

public final class UriRecipient implements android.os.Parcelable {


    /*package*/ final boolean mValid;

    /*package*/ final String mAddress;

    /*package*/ final String mLabel;

    /*package*/ final String mDesc;

    /*package*/ final long mAmount;

    public UriRecipient(
            boolean valid,
            String address,
            String label,
            String desc,
            long amount) {
        this.mValid = valid;
        this.mAddress = address;
        this.mLabel = label;
        this.mDesc = desc;
        this.mAmount = amount;
    }

    public boolean getValid() {
        return mValid;
    }

    public String getAddress() {
        return mAddress;
    }

    public String getLabel() {
        return mLabel;
    }

    public String getDesc() {
        return mDesc;
    }

    public long getAmount() {
        return mAmount;
    }

    @Override
    public String toString() {
        return "UriRecipient{" +
                "mValid=" + mValid +
                "," + "mAddress=" + mAddress +
                "," + "mLabel=" + mLabel +
                "," + "mDesc=" + mDesc +
                "," + "mAmount=" + mAmount +
        "}";
    }


    public static final android.os.Parcelable.Creator<UriRecipient> CREATOR
        = new android.os.Parcelable.Creator<UriRecipient>() {
        @Override
        public UriRecipient createFromParcel(android.os.Parcel in) {
            return new UriRecipient(in);
        }

        @Override
        public UriRecipient[] newArray(int size) {
            return new UriRecipient[size];
        }
    };

    public UriRecipient(android.os.Parcel in) {
        this.mValid = in.readByte() != 0;
        this.mAddress = in.readString();
        this.mLabel = in.readString();
        this.mDesc = in.readString();
        this.mAmount = in.readLong();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(android.os.Parcel out, int flags) {
        out.writeByte(this.mValid ? (byte)1 : 0);
        out.writeString(this.mAddress);
        out.writeString(this.mLabel);
        out.writeString(this.mDesc);
        out.writeLong(this.mAmount);
    }

}
