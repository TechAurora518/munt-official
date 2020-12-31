// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

public final class AccountRecord {


    /*package*/ final String mUUID;

    /*package*/ final String mLabel;

    /*package*/ final String mState;

    /*package*/ final String mType;

    /*package*/ final boolean mIsHD;

    public AccountRecord(
            String UUID,
            String label,
            String state,
            String type,
            boolean isHD) {
        this.mUUID = UUID;
        this.mLabel = label;
        this.mState = state;
        this.mType = type;
        this.mIsHD = isHD;
    }

    public String getUUID() {
        return mUUID;
    }

    public String getLabel() {
        return mLabel;
    }

    public String getState() {
        return mState;
    }

    public String getType() {
        return mType;
    }

    public boolean getIsHD() {
        return mIsHD;
    }

    @Override
    public String toString() {
        return "AccountRecord{" +
                "mUUID=" + mUUID +
                "," + "mLabel=" + mLabel +
                "," + "mState=" + mState +
                "," + "mType=" + mType +
                "," + "mIsHD=" + mIsHD +
        "}";
    }

}
