// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_OPTIONSMODEL_H
#define GULDEN_QT_OPTIONSMODEL_H

#include "amount.h"
#include "ticker.h"
#include "nockssettings.h"

#include <QAbstractListModel>

QT_BEGIN_NAMESPACE
class QNetworkProxy;
QT_END_NAMESPACE

class OptionsModel;
class QSettings;

enum GraphScale
{
    Blocks,
    Days,
    Weeks,
    Months
};

// Gulden specific settings go here
class GuldenOptionsModel : public QObject
{
    Q_OBJECT

public:
    GuldenOptionsModel( OptionsModel* parent );
    ~GuldenOptionsModel();
    void InitSettings(QSettings& settings);
    void setLocalCurrency(const QString &value);
    QString getLocalCurrency();
    void setWitnessGraphScale(int scale);
    int getWitnessGraphScale();
private:
    OptionsModel* m_pImpl;
    QString localCurrency;
    int witnessGraphScale;

Q_SIGNALS:
    void localCurrencyChanged(QString currency);
};

/** Interface from Qt to configuration data structure for Gulden client.
   To Qt, the options are presented as a list with the different options
   laid out vertically.
   This can be changed to a tree once the settings become sufficiently
   complex.
 */
class OptionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit OptionsModel(QObject *parent = 0, bool resetSettings = false);

    enum OptionID {
        StartAtStartup,         // bool
        AutoUpdateCheck,        // bool
        HideTrayIcon,           // bool
        MinimizeToTray,         // bool
        MapPortUPnP,            // bool
        MinimizeOnClose,        // bool
        DockOnClose,            // bool
        KeepOpenWhenMining,     // bool
        MineAtStartup,          // bool
        MineMemory,             // int
        MineThreadCount,        // int
        ProxyUse,               // bool
        ProxyIP,                // QString
        ProxyPort,              // int
        ProxyUseTor,            // bool
        ProxyIPTor,             // QString
        ProxyPortTor,           // int
        DisplayUnit,            // GuldenUnits::Unit
        AutoHideStatusBar,      //
        ThirdPartyTxUrls,       // QString
        Language,               // QString
        CoinControlFeatures,    // bool
        ThreadsScriptVerif,     // int
        DatabaseCache,          // int
        SpendZeroConfChange,    // bool
        Listen,                 // bool
        OptionIDRowCount,
        WitnessInfoUnit         // int
    };

    void Init(bool resetSettings = false);
    void Reset();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    bool setData(int index, const QVariant & value);
    /** Updates current unit in memory, settings and emits displayUnitChanged(newUnit) signal */
    void setDisplayUnit(const QVariant &value);

    /* Explicit getters */
    bool getHideTrayIcon() { return fHideTrayIcon; }
    bool getMinimizeToTray() { return fMinimizeToTray; }
    bool getDockOnClose() { return fDockOnClose; }
    bool getAutoUpdateCheck() { return fAutoUpdateCheck; }
    bool getMinimizeOnClose() { return fMinimizeOnClose; }
    bool getKeepOpenWhenMining() { return fKeepOpenWhenMining; }
    void setKeepOpenWhenMining(bool val);
    bool getMineAtStartup() { return fMineAtStartup; }
    void setMineAtStartup(bool val);
    uint64_t getMineMemory() { return nMineMemory; }
    void setMineMemory(uint64_t val);
    uint64_t getMineThreadCount() { return nMineThreadCount; }
    void setMineThreadCount(uint64_t val);
    int getDisplayUnit() { return nDisplayUnit; }
    bool getAutoHideStatusBar() const { return fAutoHideStatusBar; }
    QString getThirdPartyTxUrls() { return strThirdPartyTxUrls; }
    bool getProxySettings(QNetworkProxy& proxy) const;
    bool getCoinControlFeatures() { return fCoinControlFeatures; }
    const QString& getOverriddenByCommandLine() { return strOverriddenByCommandLine; }

    /* Restart flag helper */
    void setRestartRequired(bool fRequired);
    bool isRestartRequired();

    GuldenOptionsModel* guldenSettings;
    friend class GuldenOptionsModel;

    void setTicker(CurrencyTicker* ticker)
    {
        currencyTicker = ticker;
    }
    CurrencyTicker* getTicker()
    {
        return currencyTicker;
    }

    void setNocksSettings(NocksSettings* settings)
    {
        nocksSettings = settings;
    }
    NocksSettings* getNocksSettings()
    {
        return nocksSettings;
    }

private:
    /* Qt-only settings */
    bool fHideTrayIcon;
    bool fMinimizeToTray;
    bool fDockOnClose;
    bool fAutoUpdateCheck;
    bool fMinimizeOnClose;
    bool fKeepOpenWhenMining;
    bool fMineAtStartup;
    uint64_t nMineMemory;
    uint64_t nMineThreadCount;
    QString language;
    int nDisplayUnit;
    bool fAutoHideStatusBar;
    QString strThirdPartyTxUrls;
    bool fCoinControlFeatures;
    /* settings that were overridden by command-line */
    QString strOverriddenByCommandLine;

    CurrencyTicker* currencyTicker;
    NocksSettings* nocksSettings;

    // Add option to list of GUI options overridden through command line/config file
    void addOverriddenOption(const std::string &option);

    // Check settings version and upgrade default values if required
    void checkAndMigrate();
Q_SIGNALS:
    void displayUnitChanged(int unit);
    void coinControlFeaturesChanged(bool);
    void hideTrayIconChanged(bool);
    void autoHideStatusBarChanged(bool);
};

#endif // GULDEN_QT_OPTIONSMODEL_H
