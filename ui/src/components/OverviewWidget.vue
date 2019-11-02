<template>
     <vue-friendly-iframe v-if="url" :src="url"></vue-friendly-iframe>
</template>

<script>
import { widgetURL } from "@/config/senseBox";
export default {
  name: "OverviewWidget",
  props: {
    // TODO: Use sensebox ID from "@config/senseBox/senseBoxes" and 
    // pick the sensebox with location closest to where a user clicked. 
    senseBoxID: {type: String, default: "5dbd75c3a68df4001a658ed8",
  },
  data: () => ({
    url: widgetURL(senseBoxID),
  }),
};
</script>

<style>
.vue-friendly-iframe iframe {
  width: 400px;
  height: 600px; 
  hspace: 0px; 
  vspace: 0px; 
  frameborder: 0px; 
  scrolling:"no";
}
</style>
