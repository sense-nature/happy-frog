<template>
  <v-expansion-panels v-model="panel" class="pa-10">
    <v-expansion-panel>
      <v-expansion-panel-header>Pond Setup</v-expansion-panel-header>
      <v-expansion-panel-content>
        <p>
          You can contribute to happy-frog by providing and installing the
          necessary setup to track on of the ponds of Herzogenmatt. The total
          costs for the hardware per pond is CHF 200.-. The necessary hardware
          is:
        </p>

        <ul>
          <li>Temperature sensor</li>
          <li>pH sensor</li>
          <li>Node</li>
          <li>Battery</li>
          <li>Case</li>
        </ul>
      </v-expansion-panel-content>
    </v-expansion-panel>

    <v-expansion-panel>
      <v-expansion-panel-header>TTN Network</v-expansion-panel-header>
      <v-expansion-panel-content>
        <p>
          Happy frog highly depends on the "The Things Network" since it uses
          its network to send the pond's measurements. The Things Network is an
          open Internet of Things infrastructure supported by its members.
          Members contribute by placing gateways or running network servers.
          Together we create a secure and redundant collaborative network. The
          Things Network is growing towards a robust and stable global network,
          providing connectivity where it is needed. You can contribute by
          providing and maintaining a TTN gateway. The costs of a TTN gateway is
          CHF 150.-.
        </p>
      </v-expansion-panel-content>
    </v-expansion-panel>
  </v-expansion-panels>
</template>

<script>
export default {
  data: () => ({
    panel: 0,
  }),
};
</script>
