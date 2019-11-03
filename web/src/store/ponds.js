import {rest} from '@/api';
import Vue from 'vue';

const state = {
  data: {},
  ponds: [
    {
      center: [47.53626, 7.54981],
      senseBoxID: '5dbd82e0a68df4001a69631d',
      sensorID: '5dbd82e0a68df4001a69631f',
    },
    /*
    {
      center: [47.53623, 7.55008],
    },
    {
      center: [47.53602, 7.55013],
    },
    {
      center: [47.53592, 7.55007],
    },
    {
      center: [47.53633, 7.55043],
    },
    {
      center: [47.53615, 7.55069],
    },
    {
      center: [47.53607, 7.55093],
    },
    {
      center: [47.53591, 7.55132],
    },
    {
      center: [47.53531, 7.55114],
    },
    {
      center: [47.53534, 7.55124],
    },
    */
  ],
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
};
const mutations = {
  addData: (state, {sensorID, data}) => Vue.set(state.data, sensorID, data),
};

export default {
  namespaced: true,
  actions,
  getters,
  mutations,
  state,
};
