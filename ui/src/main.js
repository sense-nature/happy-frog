import Vue from 'vue'
import App from './App.vue'
import vuetify from './plugins/vuetify';
import router from './router/index'
import VuePlotly from '@statnett/vue-plotly';

Vue.component('vue-plotly', VuePlotly);
Vue.config.productionTip = false

new Vue({
  vuetify,
  router,
  render: h => h(App)
}).$mount('#app')
