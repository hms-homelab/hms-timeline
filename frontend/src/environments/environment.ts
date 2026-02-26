// Development environment
// Use empty string to make API calls go through the dev server proxy
export const environment = {
  production: false,
  apiUrl: '', // Empty = relative URLs, proxied to localhost:8000 by Angular dev server
};
