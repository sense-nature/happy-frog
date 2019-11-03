import {rest} from '@/api';
import Vue from 'vue';

const state = {
  data: {},
  ponds: [],
};
const getters = {
  ponds: state => state.ponds,
  data: state => state.data,
};
const actions = {
  getData: ({commit}, {sensorID, senseBoxID}) => {
    rest
      .getData({sensorID, senseBoxID})
      .then(data => commit('addData', {sensorID, data}));
  },
  getPonds: ({commit}) => {
    rest.getPonds().then(data => commit('setPonds', data));
  },
};
const mutations = {
  addData: (state, {sensorID, data}) => Vue.set(state.data, sensorID, data),
  setPonds: (state, ponds) => (state.ponds = ponds),
};

export default {
  namespaced: true,
  actions,
  getters,
  mutations,
  state,
};
