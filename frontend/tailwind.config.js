/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    "./src/**/*.{html,ts}",
  ],
  theme: {
    extend: {
      colors: {
        // Home Assistant dark theme colors
        'ha-primary': '#03a9f4',
        'ha-background': '#111827',
        'ha-card': '#1f2937',
        'ha-border': '#374151',
      },
    },
  },
  plugins: [],
}
