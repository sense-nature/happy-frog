<template>
  <vue-plotly 
    :data="data" 
    :layout="layout" 
    :options="options"
    :autoResize="true"
  />
</template>


<script>
import VuePlotly from "@statnett/vue-plotly";
import axios from "axios";
export default {
  name: "PlotlyChart",
  components: {
    VuePlotly,
  },

  props: {
    // Can be adapted to process other sensors.
    sensorID: {type: String, default: "5dbd75c3a68df4001a658edb"},
  },

  data: function() {
    return {
      data: [],
      layout: {
        xaxis: {
          title: "Date",
          autorange: true,
          range: ['2019-02-17', '2019-12-16'],
          rangeselector: {buttons: [
              {
                count: 1,
                label: '1d',
                step: 'day',
                stepmode: 'backward'
              },
              {
                count: 1,
                label: '1m',
                step: 'month',
                stepmode: 'backward'
              },
              {step: 'all'}
            ]},
          type: 'date'
        },

        yaxis: {
          title: "Temperature °C",
        },

     },
   options: {
     responsive: true,
     showLink: false,
     displayModeBar: false,
   },
   };
  },

  mounted() {
    axios.get("https://api.opensensemap.org/boxes/5dbd75c3a68df4001a658ed8/data/" + this.sensorID).then(
      response => (this.data = [{
        x: response.data.map((row) => row.createdAt),
        y: response.data.map((row) => row.value), 
        line: {color: '#17BECF'}
      }]) 
    );
  },
};
</script>
