#ifndef BITCOIN_WALLET_WALLETDBERRORS_H
#define BITCOIN_WALLET_WALLETDBERRORS_H

/** Error statuses for the wallet database */
enum DBErrors
{
    DB_LOAD_OK,
    DB_CORRUPT,
    DB_NONCRITICAL_ERROR,
    DB_TOO_NEW,
    DB_LOAD_FAIL,
    DB_NEED_REWRITE
};


#endif // BITCOIN_WALLET_WALLETDBERRORS_H
