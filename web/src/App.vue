<template>
  <v-app>
    <Navigation />

    <v-content>
      <router-view />
    </v-content>
  </v-app>
</template>

<script>
import Navigation from '@/components/Layout/Navigation';

export default {
  components: {
    Navigation,
  },
  watch: {
    $route(to) {
      document.title = `${to.meta.title} - Happy Frog` || 'Happy Frog';
    },
  },
  created() {
    document.title = `${this.$route.meta.title} - Happy Frog`;
  },
};
</script>
