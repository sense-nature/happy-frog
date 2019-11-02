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
        <v-btn to="/detail">Detail</v-btn>
      </v-flex>

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
    
  }),
};
</script>
