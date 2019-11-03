<template>
  <v-card>
    <VuePlotly
      v-if="pond !== null"
      :data="data[pond.sensorID]"
      :layout="layout"
      :options="options"
      autoResize
    />
    <v-card-text v-else>
      No pond selected
    </v-card-text>
  </v-card>
</template>

<script>
import VuePlotly from '@statnett/vue-plotly';
import {mapGetters} from 'vuex';

export default {
  components: {
    VuePlotly,
  },
  computed: {
    ...mapGetters('ponds', ['data']),
  },
  data: () => ({
    layout: {
      xaxis: {
        title: 'Date',
        autorange: true,
        range: ['2019-02-17', '2019-12-16'],
        rangeselector: {
          buttons: [
            {
              count: 1,
              label: '1d',
              step: 'day',
              stepmode: 'backward',
            },
            {
              count: 1,
              label: '1m',
              step: 'month',
              stepmode: 'backward',
            },
            {step: 'all'},
          ],
        },
        type: 'date',
      },
      yaxis: {
        title: 'Temperature °C',
      },
    },
    options: {
      responsive: true,
      showLink: false,
      displayModeBar: false,
    },
  }),
  props: ['pond'],
};
</script>
