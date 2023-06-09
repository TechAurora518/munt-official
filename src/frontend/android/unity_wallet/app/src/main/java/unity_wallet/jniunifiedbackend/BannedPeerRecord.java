// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package unity_wallet.jniunifiedbackend;

public final class BannedPeerRecord {


    /*package*/ final String mAddress;

    /*package*/ final long mBannedUntil;

    /*package*/ final long mBannedFrom;

    /*package*/ final String mReason;

    public BannedPeerRecord(
            String address,
            long bannedUntil,
            long bannedFrom,
            String reason) {
        this.mAddress = address;
        this.mBannedUntil = bannedUntil;
        this.mBannedFrom = bannedFrom;
        this.mReason = reason;
    }

    public String getAddress() {
        return mAddress;
    }

    public long getBannedUntil() {
        return mBannedUntil;
    }

    public long getBannedFrom() {
        return mBannedFrom;
    }

    public String getReason() {
        return mReason;
    }

    @Override
    public String toString() {
        return "BannedPeerRecord{" +
                "mAddress=" + mAddress +
                "," + "mBannedUntil=" + mBannedUntil +
                "," + "mBannedFrom=" + mBannedFrom +
                "," + "mReason=" + mReason +
        "}";
    }

}
