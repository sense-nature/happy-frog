<template>
  <v-app>
    <nav>
      <v-toolbar app>
          <Logo class="logo"/>
        <v-toolbar-title class="headline text-uppercase">
          <span class="font-weight-light">Lepond</span>
        </v-toolbar-title>
        <v-spacer></v-spacer>
        <v-toolbar-items class="hidden-sm-and-down">
          <v-btn class="mx-1" to="/">Home</v-btn>
          <v-btn class="mx-2" to="/about">About</v-btn>
        </v-toolbar-items>
      </v-toolbar>
    </nav>

    <v-content>
      <router-view />
    </v-content>
  </v-app>
</template>

<script>
import Logo from './components/logo.vue';
export default {
  name: "App",
  components: {
    Logo,
  },
  data() {
    return {
      //
    };
  }
};
</script>
