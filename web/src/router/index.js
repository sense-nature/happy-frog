import Vue from 'vue';
import VueRouter from 'vue-router';
import About from '@/views/About';
import Contribute from '@/views/Contribute';
import Ponds from '@/views/Ponds';

Vue.use(VueRouter);

const routes = [
  {
    name: 'ponds',
    path: '/ponds',
    component: Ponds,
    meta: {icon: 'mdi-waves', title: 'Ponds', navigation: true},
  },
  {
    name: 'contribute',
    path: '/contribute',
    component: Contribute,
    meta: {icon: 'mdi-hand-heart', title: 'Contribute', navigation: true},
  },
  {
    name: 'about',
    path: '/about',
    component: About,
    meta: {icon: 'mdi-information', title: 'About', navigation: true},
  },
  {
    path: '/',
    redirect: {name: 'ponds'},
  },
];

const router = new VueRouter({
  routes,
});

export default router;
