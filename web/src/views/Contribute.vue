<template>
  <v-expansion-panels v-model="panels" class="pa-10">
    <v-expansion-panel>
      <v-expansion-panel-header>TTN Network</v-expansion-panel-header>
      <v-expansion-panel-content> </v-expansion-panel-content>
    </v-expansion-panel>

    <v-expansion-panel>
      <v-expansion-panel-header>Pond Setup</v-expansion-panel-header>
      <v-expansion-panel-content> </v-expansion-panel-content>
    </v-expansion-panel>

    <v-expansion-panel>
      <v-expansion-panel-header>Donations</v-expansion-panel-header>
      <v-expansion-panel-content> </v-expansion-panel-content>
    </v-expansion-panel>
  </v-expansion-panels>
</template>

<script>
export default {
  data: () => ({
    panels: [0],
  }),
};
</script>
