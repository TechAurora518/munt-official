// Modules to control application life and create native browser window
const {app, BrowserWindow} = require('electron')

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let mainWindow

function createWindow () {
  // Create the browser window.
  mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      nodeIntegration: true
    }
  })

  // and load the index.html of the app.
  mainWindow.loadFile('index.html')

  // Open the DevTools.
   mainWindow.webContents.openDevTools()

  // Emitted when the window is closed.
  mainWindow.on('closed', function () {
    // Dereference the window object, usually you would store windows
    // in an array if your app supports multi windows, this is the time
    // when you should delete the corresponding element.
    mainWindow = null
  })

  guldenUnitySetup();
}

// Keep global references of all these objects
let libgulden
let guldenbackend
let guldensignalhandler
let signalhandler
function guldenUnitySetup() {
    var basepath = app.getAppPath();

    global.libgulden = libgulden = require('./libgulden_unity_node_js')
    global.guldenbackend = guldenbackend = new libgulden.NJSGuldenUnifiedBackend
    global.signalhandler = signalhandler = new Object()
    global.guldensignalhandler = guldensignalhandler = new libgulden.NJSGuldenUnifiedFrontend(signalhandler);

    // Receive signals from the core and marshall them as needed to the main window
    signalhandler.notifyCoreReady = function() {
        console.log("received: notifyCoreReady")
        mainWindow.webContents.send('notifyCoreReady')
    }
    signalhandler.logPrint  = function(message) {
        console.log("gulden_unity_core: " + message)
        mainWindow.webContents.send('logPrint', message)
    }
    signalhandler.notifyUnifiedProgress  = function (progress) {
        console.log("received: notifyUnifiedProgress")
        mainWindow.webContents.send('notifyUnifiedProgress', progress)
    }
    signalhandler.notifyBalanceChange = function (new_balance) {
        console.log("received: notifyBalanceChange")
        mainWindow.webContents.send('notifyBalanceChange', new_balance)
    }
    signalhandler.notifyNewMutation  = function (mutation, self_committed) {
        console.log("received: notifyNewMutation")
        mainWindow.webContents.send('notifyNewMutation', mutation, self_committed)
    }
    signalhandler.notifyUpdatedTransaction  = function (transaction) {
        console.log("received: notifyUpdatedTransaction")
        mainWindow.webContents.send('notifyUpdatedTransaction', transaction)
    }
    signalhandler.notifyInitWithExistingWallet = function () {
        console.log("received: notifyInitWithExistingWallet")
        mainWindow.webContents.send('notifyInitWithExistingWallet')
    }
    signalhandler.notifyInitWithoutExistingWallet = function () {
        console.log("received: notifyInitWithoutExistingWallet")
        mainWindow.webContents.send('notifyInitWithoutExistingWallet')
        guldenbackend.InitWalletFromRecoveryPhrase(guldenbackend.GenerateRecoveryMnemonic(),"password")
    }
    signalhandler.notifyShutdown = function () {
        console.log("received: notifyShutdown")
        mainWindow.webContents.send('notifyShutdown')
    }

    // Start the Gulden unified backend
    guldenbackend.InitUnityLibThreaded(basepath+"/"+"wallet", "", -1, -1, false, guldensignalhandler, "")
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', function () {
  // On macOS it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  if (process.platform !== 'darwin') app.quit()
})

app.on('activate', function () {
  // On macOS it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (mainWindow === null) createWindow()
})

