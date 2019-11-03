<template>
  <v-container>
    <v-row>
      <v-col cols="6">
        <LMap class="map" ref="map" :center="[47.5358, 7.55047]" :zoom="18">
          <LTileLayer :url="'http://{s}.tile.osm.org/{z}/{x}/{y}.png'" />
          <LMarker
            :lat-lng="item.center"
            v-for="(item, key) in ponds"
            :key="key"
            @click="select(item)"
          />
        </LMap>
      </v-col>
      <v-col cols="6">
        <PondPlot :pond="selected" />
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import PondPlot from '@/components/PondPlot';
import {LMap, LMarker, LTileLayer} from 'vue2-leaflet';
import {mapActions, mapGetters} from 'vuex';

export default {
  components: {
    LMap,
    LMarker,
    LTileLayer,
    PondPlot,
  },
  computed: {
    ...mapGetters('ponds', ['ponds', 'data']),
  },
  data: () => ({
    selected: null,
  }),
  methods: {
    ...mapActions('ponds', ['getData', 'getPonds']),
    getPondsData() {
      this.ponds.forEach(pond => {
        this.getData({sensorID: pond.sensorID, senseBoxID: pond.senseBoxID});
      });
    },
    select(item) {
      this.selected = item;
      this.$refs.map.mapObject.panTo(item.center);
    },
  },
  beforeMount() {
    this.getPonds().then(this.getPondsData);
  },
};
</script>

<style scoped>
.map {
  height: calc(100vh - 150px);
}
</style>
