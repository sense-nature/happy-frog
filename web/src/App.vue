<template>
  <v-app>
    <Header />

    <Navigation />

    <v-content>
      <router-view />
    </v-content>

    <Footer />
  </v-app>
</template>

<script>
import Footer from '@/components/Layout/Footer';
import Header from '@/components/Layout/Header';
import Navigation from '@/components/Layout/Navigation';

export default {
  components: {
    Footer,
    Header,
    Navigation,
  },
};
</script>
