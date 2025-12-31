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
  const code = req.query.code || null;
  const state = req.query.state || null;

  if (!code || !state) {
    return res.status(400).send('Error: Missing code or state.');
  }

  try {
    const decodedState = JSON.parse(Buffer.from(state, 'base64').toString());
    const { device_id, client_id, client_secret } = decodedState;

    if (!device_id || !client_id || !client_secret) {
      throw new Error('Invalid state data.');
    }

    const tokenResponse = await axios({
      method: 'post',
      url: 'https://accounts.spotify.com/api/token',
      data: querystring.stringify({
        code: code,
        redirect_uri: `https://${req.headers.host}/api/callback`,
        grant_type: 'authorization_code',
      }),
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        'Authorization': 'Basic ' + Buffer.from(`${client_id}:${client_secret}`).toString('base64'),
      },
    });

    const refresh_token = tokenResponse.data.refresh_token;

    const client = await pool.connect();
    await client.query(
      `
      INSERT INTO devices (device_id, refresh_token, client_id, client_secret)
      VALUES ($1, $2, $3, $4)
      ON CONFLICT (device_id)
      DO UPDATE SET 
        refresh_token = $2, 
        client_id = $3, 
        client_secret = $4, 
        updated_at = NOW();
    `,
      [device_id, refresh_token, client_id, client_secret]
    );
    client.release();

    res.send(`
      <body style="background:#121212; color:white; font-family:sans-serif; text-align:center; padding-top:50px;">
        <h1 style="color:#1DB954;">Success!</h1>
        <p>Device <strong>${device_id}</strong> is now linked to your custom Spotify App.</p>
        <p>You can restart your ESP32 now.</p>
      </body>
    `);
  } catch (error) {
    console.error('Setup Error:', error.response ? error.response.data : error.message);
    res
      .status(500)
      .send(
        'Error during setup: ' +
          (error.response ? JSON.stringify(error.response.data) : error.message)
      );
  }
};
