/**
 * Extract a relative media path from a recording or snapshot URL.
 * Handles both formats stored in the DB:
 *   - Old (Python backend): "http://192.168.2.5:8000/events/filename.mp4"
 *   - New (C++ backend):    "filename.mp4"
 *
 * Returns a relative path (no leading /) so it resolves against <base href>
 * set by HA ingress, e.g. "events/filename.mp4" or "snapshots/filename.jpg".
 */
export function toRelativeMediaUrl(
  rawUrl: string | undefined | null,
  prefix: 'events' | 'snapshots'
): string {
  if (!rawUrl) {
    return '';
  }

  let filename: string;

  if (rawUrl.startsWith('http://') || rawUrl.startsWith('https://')) {
    // Old format: full URL — extract just the filename from the path
    try {
      const url = new URL(rawUrl);
      const parts = url.pathname.split('/');
      filename = parts[parts.length - 1];
    } catch {
      filename = rawUrl;
    }
  } else if (rawUrl.includes('/')) {
    // Path like "/events/filename.mp4" or "events/filename.mp4"
    const parts = rawUrl.split('/');
    filename = parts[parts.length - 1];
  } else {
    // New format: bare filename
    filename = rawUrl;
  }

  if (!filename) {
    return '';
  }

  return `${prefix}/${filename}`;
}
