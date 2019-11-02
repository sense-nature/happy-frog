<template>
  <v-container>
    <v-row>
      <v-col :cols="6">
        <LMap class="map" ref="map" :center="[47.5358, 7.55047]" :zoom="18">
          <LTileLayer :url="'http://{s}.tile.osm.org/{z}/{x}/{y}.png'" />
          <LMarker
            :lat-lng="item.center"
            v-for="(item, key) in items"
            :key="key"
            @click="select(item)"
          />
        </LMap>
      </v-col>
      <v-col cols="6">
        {{ selected }}
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import {LMap, LMarker, LTileLayer} from 'vue2-leaflet';

export default {
  components: {
    LMap,
    LMarker,
    LTileLayer,
  },
  data: () => ({
    items: [
      {
        center: [47.53626, 7.54981],
      },
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
    ],
    selected: null,
  }),
  methods: {
    select(item) {
      this.selected = item;
      this.$refs.map.mapObject.panTo(item.center);
    },
  },
};
</script>

<style scoped>
.map {
  height: calc(100vh - 150px);
}
</style>
