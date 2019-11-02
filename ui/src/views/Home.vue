<template>
  <div>
    <PlotlyChart/>
  </div>
</template>

<script>
// @ is an alias to /src
import PlotlyChart from '@/components/PlotlyChart.vue'

export default {
  name: 'home',
  components: {
    PlotlyChart
  }
}
</script>
