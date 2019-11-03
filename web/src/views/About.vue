<template>
  <v-container>
    <v-card class="pa-10">
      <v-img
        src="https://github.com/escodebar/happy-frog/blob/master/architecture_happyfrog.png?raw=true"
      />

      <v-card-text>
        This project was started to measure the water temperature and pH values
        of the pond at the
        <a href="https://www.herzogenmatt.ch/">Herzogenmatt natural reserve</a>.
      </v-card-text>

      <v-card-text>
        Sensors are submerged into ponds to measure various properties of the
        water such as pH and temperature. We use TTN community based LoRaWAN
        network to transfer the data from the ponds into the cloud. We built our
        own TTN Gateway to collect the data from the remote sensors. Sensors
        closer to civilization can use standard WiFi to transfer the data. The
        data is made publicly available through the openSenseMap, an open data
        hosting provider. A web application gives an overview of the ponds and
        gives access to the data and inform the user of how to contribute to
        these measurements. The web application is hosted in the Google Cloud.
        Continuous deployment is made with Travis upon changes on the public
        Github repository.
      </v-card-text>

      <v-card-text>
        To contribute to this project, please visit:
        <a href="https://github.com/escodebar/happy-frog"
          >https://github.com/escodebar/happy-frog</a
        >
      </v-card-text>
    </v-card>
  </v-container>
</template>
