<template>
  <v-app>
    <nav>
      <v-toolbar app>
        <v-toolbar-title class="headline text-uppercase">
          <span class="font-weight-light">Testpage</span>
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
export default {
  name: "App",
  components: {},
  data() {
    return {
      //
    };
  }
};
</script>
