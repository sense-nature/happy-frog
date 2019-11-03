import {rest} from '@/api';
import Vue from 'vue';

const state = {
  data: {},
  ponds: [],
  sensors: {},
};
const getters = {
  ponds: state => state.ponds,
  data: state => state.data,
  sensors: state => state.sensors,
};
const actions = {
  getData: ({commit}, item) =>
    rest
      .getData(item)
      .then(payloads =>
        payloads.forEach(payload => commit('addData', payload)),
      ),
  getPonds: ({commit}) =>
    rest.getPonds().then(data => {
      commit(
        'setSensors',
        data
          .reduce((acc, pond) => acc.concat(pond.sensors), [])
          .reduce((acc, sensor) => {
            acc[sensor._id] = sensor;
            return acc;
          }, {}),
      );
      commit('setPonds', data);
    }),
};
const mutations = {
  addData: (state, {sensorID, data}) => Vue.set(state.data, sensorID, data),
  setPonds: (state, ponds) => (state.ponds = ponds),
  setSensors: (state, sensors) => (state.sensors = sensors),
};

export default {
  namespaced: true,
  actions,
  getters,
  mutations,
  state,
};
