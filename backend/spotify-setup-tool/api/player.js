const axios = require('axios');
const querystring = require('querystring');
const { Pool } = require('pg');

const connectionString = process.env.POSTGRES_URL ? process.env.POSTGRES_URL.split('?')[0] : null;

const pool = new Pool({
  connectionString: connectionString,
  ssl: {
    rejectUnauthorized: false,
  },
});

module.exports = async (req, res) => {
  const device_id = req.query.device_id;
  if (!device_id) return res.status(400).json({ error: 'Missing device_id' });

  try {
    const client = await pool.connect();
    const result = await client.query(
      'SELECT refresh_token, client_id, client_secret FROM devices WHERE device_id = $1',
      [device_id]
    );
    client.release();

    if (result.rows.length === 0) {
      return res.status(404).json({ error: 'Device not registered', is_configured: false });
    }

    const { refresh_token, client_id, client_secret } = result.rows[0];

    const tokenRes = await axios({
      method: 'post',
      url: 'https://accounts.spotify.com/api/token',
      data: querystring.stringify({
        grant_type: 'refresh_token',
        refresh_token: refresh_token,
      }),
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        'Authorization': 'Basic ' + Buffer.from(`${client_id}:${client_secret}`).toString('base64'),
      },
    });

    const access_token = tokenRes.data.access_token;

    const playerRes = await axios.get('https://api.spotify.com/v1/me/player', {
      headers: { 'Authorization': `Bearer ${access_token}` },
    });

    if (playerRes.status === 204 || !playerRes.data || !playerRes.data.item) {
      return res.json({ is_playing: false, is_configured: true });
    }

    const item = playerRes.data.item;
    const progressMs = playerRes.data.progress_ms;
    const durationMs = item.duration_ms;
    const artists = item.artists.map(a => a.name).join(', ');

    res.json({
      is_configured: true,
      is_playing: playerRes.data.is_playing,
      title: item.name,
      artist: artists,
      duration: durationMs,
      progress: progressMs,
    });
  } catch (error) {
    console.error('API Error:', error.response ? error.response.data : error.message);
    res.status(500).json({ error: error.message });
  }
};
