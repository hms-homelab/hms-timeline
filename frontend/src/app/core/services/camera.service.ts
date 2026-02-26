import { Injectable, inject } from '@angular/core';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { ApiService } from './api.service';
import { Camera, CameraStatus } from '../models/camera.model';

/**
 * Service for camera-related operations
 * Fetches camera status, health, and configuration
 */
@Injectable({
  providedIn: 'root'
})
export class CameraService {
  private api = inject(ApiService);

  /**
   * Get enhanced camera status with last event time
   */
  getCameraStatus(): Observable<Camera[]> {
    return this.api.get<CameraStatus>('api/cameras/status').pipe(
      map(response => response.cameras)
    );
  }

  /**
   * Get system health check (includes camera buffer stats)
   */
  getHealth(): Observable<any> {
    return this.api.get('health');
  }

  /**
   * Get camera list (simpler endpoint without last event)
   */
  getCameras(): Observable<Camera[]> {
    return this.api.get<{ cameras: Camera[] }>('cameras').pipe(
      map(response => response.cameras.map(cam => ({
        id: cam.id,
        name: cam.name,
        connected: cam.connected,
        buffer_size: cam.buffer_size
      })))
    );
  }
}
