import axios from 'axios';

const BASE_URL = 'https://api.opensensemap.org/boxes/';

export default {
  getData: item =>
    axios
      .get(`${BASE_URL}${item.senseBoxID}/data/${item.sensorID}`)
      .then(response => [
        {
          x: response.data.map(row => row.createdAt),
          y: response.data.map(row => row.value),
          line: {color: '#17BECF'},
        },
      ]),
};
