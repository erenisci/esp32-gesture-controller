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
  const device_id = req.query.state || null;

  if (!code || !device_id) {
    return res.send('Error: Missing code or device_id.');
  }

  try {
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
        'Authorization':
          'Basic ' +
          Buffer.from(
            process.env.SPOTIFY_CLIENT_ID + ':' + process.env.SPOTIFY_CLIENT_SECRET
          ).toString('base64'),
      },
    });

    const refresh_token = tokenResponse.data.refresh_token;

    const client = await pool.connect();
    await client.query(
      `
      INSERT INTO devices (device_id, refresh_token)
      VALUES ($1, $2)
      ON CONFLICT (device_id)
      DO UPDATE SET refresh_token = $2, updated_at = NOW();
    `,
      [device_id, refresh_token]
    );
    client.release();

    res.send(`
      <body style="background:#121212; color:white; font-family:sans-serif; text-align:center; padding-top:50px;">
        <h1 style="color:#1DB954;">Success!</h1>
        <p>Device <strong>${device_id}</strong> is now linked.</p>
        <p>You can restart your ESP32 now.</p>
      </body>
    `);
  } catch (error) {
    console.error('Setup Error:', error.message);
    res.send('Error during setup: ' + error.message);
  }
};
