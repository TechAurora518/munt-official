const wallet = {
  namespaced: true,
  state: {
    accounts: [],
    activeAccount: null,
    balance: null,
    mutations: null,
    receiveAddress: null,
    walletBalance: null,
    walletPassword: null,
    unlocked: false
  },
  mutations: {
    SET_ACCOUNTS(state, accounts) {
      state.accounts = accounts;
    },
    SET_ACTIVE_ACCOUNT(state, accountUUID) {
      state.activeAccount = accountUUID;
    },
    SET_BALANCE(state, balance) {
      state.balance = balance;
    },
    SET_MUTATIONS(state, mutations) {
      state.mutations = mutations;
    },
    SET_RECEIVE_ADDRESS(state, receiveAddress) {
      state.receiveAddress = receiveAddress;
    },
    SET_WALLET_BALANCE(state, walletBalance) {
      state.walletBalance = walletBalance;
    },
    SET_WALLET_PASSWORD(state, password) {
      state.walletPassword = password;
    },
    SET_UNLOCKED(state, unlocked) {
      state.unlocked = unlocked;
    },
    SET_WALLET(state, payload) {
      // batch update state properties from payload
      for (const [key, value] of Object.entries(payload)) {
        state[key] = value;
      }
    }
  },
  actions: {
    SET_ACCOUNT_NAME({ state, commit }, payload) {
      let accounts = [...state.accounts];
      let account = accounts.find(x => x.UUID === payload.accountUUID);
      account.label = payload.newAccountName;
      commit("SET_ACCOUNTS", accounts);
    },
    SET_ACCOUNTS({ commit }, accounts) {
      commit("SET_ACCOUNTS", accounts);
    },
    SET_ACTIVE_ACCOUNT({ commit }, accountUUID) {
      // clear mutations and receive address
      commit("SET_RECEIVE_ADDRESS", { receiveAddress: "" });
      commit("SET_ACTIVE_ACCOUNT", accountUUID);
      commit("SET_MUTATIONS", { mutations: null });
    },
    SET_BALANCE({ commit }, new_balance) {
      commit("SET_BALANCE", new_balance);
    },
    SET_MUTATIONS({ commit }, mutations) {
      commit("SET_MUTATIONS", mutations);
    },
    SET_RECEIVE_ADDRESS({ commit }, receiveAddress) {
      commit("SET_RECEIVE_ADDRESS", receiveAddress);
    },
    SET_WALLET_BALANCE({ commit }, walletBalance) {
      commit("SET_WALLET_BALANCE", walletBalance);
    },
    SET_WALLET_PASSWORD({ commit }, password) {
      commit("SET_WALLET_PASSWORD", password);
    },
    SET_UNLOCKED({ commit }, unlocked) {
      commit("SET_UNLOCKED", unlocked);
    },
    SET_WALLET({ commit }, payload) {
      commit("SET_WALLET", payload);
    }
  },
  getters: {
    totalBalance: state => {
      let balance = state.walletBalance;
      if (balance === undefined || balance === null) return null;
      return balance.availableIncludingLocked + balance.unconfirmedIncludingLocked + balance.immatureIncludingLocked;
    },
    lockedBalance: state => {
      let balance = state.walletBalance;
      if (balance === undefined || balance === null) return null;
      return balance.totalLocked;
    },
    spendableBalance: state => {
      let balance = state.walletBalance;
      if (balance === undefined || balance === null) return null;
      return balance.availableExcludingLocked;
    },
    pendingBalance: state => {
      let balance = state.walletBalance;
      if (balance === undefined || balance === null) return null;
      return balance.unconfirmedExcludingLocked;
    },
    immatureBalance: state => {
      let balance = state.walletBalance;
      if (balance === undefined || balance === null) return null;
      return balance.immatureIncludingLocked;
    },
    accounts: state => {
      return state.accounts
        .filter(x => x.state === "Normal")
        .sort((a, b) => {
          const labelA = a.label.toUpperCase();
          const labelB = b.label.toUpperCase();

          let comparison = 0;
          if (labelA > labelB) {
            comparison = 1;
          } else if (labelA < labelB) {
            comparison = -1;
          }
          return comparison;
        });
    },
    account: state => {
      return state.accounts.find(x => x.UUID === state.activeAccount);
    }
  }
};

export default wallet;
