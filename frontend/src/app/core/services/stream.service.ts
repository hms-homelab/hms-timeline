import { Injectable } from '@angular/core';
import { environment } from '../../../environments/environment';

/**
 * Service for camera snapshot URLs
 * Provides URL construction for live camera snapshots from buffer
 */
@Injectable({
  providedIn: 'root'
})
export class StreamService {
  private apiUrl = environment.apiUrl;

  /**
   * Get live snapshot URL for a camera
   * Returns the most recent frame from the camera buffer
   * @param cameraId Camera identifier (e.g., 'patio', 'side_window', 'front_door')
   * @returns Snapshot URL
   */
  getLiveSnapshotUrl(cameraId: string): string {
    return `api/cameras/${cameraId}/snapshot`;
  }

  /**
   * Get cached snapshot URL for an event
   * @param filename Snapshot filename (e.g., 'patio_20260222_134500.jpg')
   * @returns Event snapshot URL
   */
  getEventSnapshotUrl(filename: string): string {
    return `snapshots/${filename}`;
  }

  /**
   * Get recording URL for an event
   * @param filename Recording filename (e.g., 'patio_20260222_134500.mp4')
   * @returns Event recording URL
   */
  getRecordingUrl(filename: string): string {
    return `events/${filename}`;
  }
}
