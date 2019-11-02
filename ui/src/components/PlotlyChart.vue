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
import { senseBoxID } from "@/config/senseBox";
export default {
  name: "PlotlyChart",
  components: {
    VuePlotly,
  },

  props: {
    // TODO: Use sensebox ID from "@config/senseBox/senseBoxes" and 
    // pick the sensebox with location closest to where a user clicked. 
    // Then, retrieve the corresponding sensors to plot information for using
    // "@config/senseBox/retrieveSensors". 
    senseBoxID: {type: String, default: "5dbd75c3a68df4001a658ed8",
    sensorID: {type: String, default: "5dbd82e0a68df4001a69631f"},
  },

  data: function() {
    return {
      data: [],
      senseBoxName: "",
      sensorType: "",
      measurementUnit: "",
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
    axios.get("https://api.opensensemap.org/boxes/" + senseBoxID + "/data/" + this.sensorID).then(
      response => (this.data = [{
        x: response.data.map((row) => row.createdAt),
        y: response.data.map((row) => row.value), 
        line: {color: '#17BECF'}
      }]) 
    );
  },
};
</script>
