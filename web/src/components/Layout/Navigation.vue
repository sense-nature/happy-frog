<template>
  <v-navigation-drawer permanent app>
    <v-list-item>
      <v-list-item-content>
        <v-list-item-title class="title">
          Happy Frog
        </v-list-item-title>
        <v-list-item-subtitle>
          bringing love to amphibians
        </v-list-item-subtitle>
      </v-list-item-content>
    </v-list-item>

    <v-divider></v-divider>

    <v-list dense nav>
      <v-list-item v-for="item in items" :key="item.name" link :to="item.path">
        <v-list-item-icon>
          <v-icon>{{ item.meta.icon }}</v-icon>
        </v-list-item-icon>

        <v-list-item-content>
          <v-list-item-title>{{ item.meta.title }}</v-list-item-title>
        </v-list-item-content>
      </v-list-item>
    </v-list>
  </v-navigation-drawer>
</template>

<script>
export default {
  computed: {
    items() {
      return this.$router.options.routes.filter(
        route => 'meta' in route && route.meta.navigation,
      );
    },
  },
};
</script>
