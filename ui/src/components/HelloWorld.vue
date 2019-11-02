<template>
  <v-container>
    <v-layout
      text-center
      wrap
    >
      <v-flex mb-4>
        <h1 class="display-2 font-weight-bold mb-3">
          Pond Overview  
        </h1>
        <Map class="map"/>
      </v-flex>
      <v-flex>
        <v-dialog
      v-model="dialog"
      width="90vw"
    >
      <template v-slot:activator="{ on }">
        <v-btn v-on="on">Detail</v-btn>
      </template>
      <v-card>
        <v-card-title
          class="headline grey lighten-2"
          primary-title
        >
          PONDY the happiest pond in west pond cluster
        </v-card-title>

        <v-card-text>
          HERE IS AN AMAZING CHART
        </v-card-text>
      </v-card>
    </v-dialog>
      </v-flex>
    <v-snackbar
    color="error"
        v-model="snackbar"
        >
        <strong>{{ text }}</strong>
        <v-btn
        text
          @click="snackbar = false"
        >
          Close
        </v-btn>
    </v-snackbar>
    </v-layout>
  </v-container>
</template>

<script>
import Map from './map.vue';
export default {
  components: {
    Map,
  },
  data: () => ({
    snackbar: false,
    text: "ALERT: Pondy is too hot",
  }),
  mounted: function () {
  this.$nextTick(function () {
    setInterval(() => this.snackbar = true, 10000);
  })
}
};
</script>
