<template>
  <v-container>
    <v-row>
      <v-col cols="6">
        <v-card height="100%">
          <LMap ref="map" :center="[47.5358, 7.55047]" :zoom="18">
            <LTileLayer
              :url="'http://{s}.tile.osm.org/{z}/{x}/{y}.png'"
            />
            <LMarker
              :lat-lng="item.center"
              v-for="(item, key) in ponds"
              :key="key"
              @click="select(item)"
            />
          </LMap>
        </v-card>
      </v-col>
      <v-col cols="6">
        <v-card>
          <PondPlot
            :sensor="sensor"
            v-for="sensor in sensorIDs"
            :key="sensor"
          />
        </v-card>
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
    sensorIDs() {
      return this.selected === null ? [] : this.selected.sensorIDs;
    },
  },
  data: () => ({
    selected: null,
    intervalResize: null,
    intervalData: null,
  }),
  methods: {
    ...mapActions('ponds', ['getData', 'getPonds']),
    getPondsData() {
      this.ponds.forEach(pond => {
        this.getData(pond);
      });
    },
    select(item) {
      this.selected = item;
      this.$refs.map.mapObject.panTo(item.center);
    },
  },
  watch: {
    ponds() {
      if (this.ponds.length > 0 && this.selected === null) {
        this.selected = this.ponds[0];
      }
    },
  },
  beforeMount() {
    this.getPonds().then(this.getPondsData);
  },
  created() {
    this.intervalResize = setInterval(() => {
      window.dispatchEvent(new Event('resize'));
    }, 1000);
    this.intervalData = setInterval(() => {
      this.getPondsData();
    }, 30000);
  },
  beforeDestroy() {
    clearInterval(this.intervalResize);
    clearInterval(this.intervalData);
    this.intervalResize = null;
    this.intervalData = null;
  },
};
</script>
