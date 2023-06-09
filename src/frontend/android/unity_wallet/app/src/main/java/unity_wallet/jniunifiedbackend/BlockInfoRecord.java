// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package unity_wallet.jniunifiedbackend;

public final class BlockInfoRecord {


    /*package*/ final int mHeight;

    /*package*/ final long mTimeStamp;

    /*package*/ final String mBlockHash;

    public BlockInfoRecord(
            int height,
            long timeStamp,
            String blockHash) {
        this.mHeight = height;
        this.mTimeStamp = timeStamp;
        this.mBlockHash = blockHash;
    }

    public int getHeight() {
        return mHeight;
    }

    public long getTimeStamp() {
        return mTimeStamp;
    }

    public String getBlockHash() {
        return mBlockHash;
    }

    @Override
    public String toString() {
        return "BlockInfoRecord{" +
                "mHeight=" + mHeight +
                "," + "mTimeStamp=" + mTimeStamp +
                "," + "mBlockHash=" + mBlockHash +
        "}";
    }

}
