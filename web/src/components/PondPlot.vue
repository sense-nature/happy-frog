<template>
  <VuePlotly
    v-if="sensor !== null"
    :data="data[sensor]"
    :layout="layout"
    :options="options"
    autoResize
  />
</template>

<script>
import VuePlotly from '@statnett/vue-plotly';
import {mapGetters} from 'vuex';

export default {
  components: {
    VuePlotly,
  },
  computed: {
    ...mapGetters('ponds', ['data', 'sensors']),
    layout() {
      return {
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
          title: `${this.sensors[this.sensor].title} ${this.sensors[this.sensor].unit}`,
        },
      };
    },
  },
  data: () => ({
    options: {
      responsive: true,
      showLink: false,
      displayModeBar: false,
    },
  }),
  props: ['sensor'],
};
</script>
