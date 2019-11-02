<template>
     <vue-friendly-iframe v-if="url" :src="url"></vue-friendly-iframe>
</template>

<script>
import { widgetURL } from "@/config/senseBox";
export default {
  name: "OverviewWidget",
  data: () => ({
    url: widgetURL,
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
