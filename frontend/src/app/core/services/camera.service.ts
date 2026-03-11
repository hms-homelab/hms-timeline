import { Injectable, inject } from '@angular/core';
import { Observable, forkJoin, of } from 'rxjs';
import { map, switchMap, catchError } from 'rxjs/operators';
import { ApiService } from './api.service';
import { Camera, CameraStatus } from '../models/camera.model';

@Injectable({
  providedIn: 'root'
})
export class CameraService {
  private api = inject(ApiService);

  getCameraStatus(): Observable<Camera[]> {
    return this.api.get<CameraStatus>('api/cameras/status').pipe(
      map(response => response.cameras),
      switchMap(cameras => {
        if (cameras.length === 0) return of(cameras);
        const pauseChecks = cameras.map(cam =>
          this.api.get<{ paused: boolean }>(`api/cameras/${cam.id}/paused`).pipe(
            map(r => ({ ...cam, paused: r.paused })),
            catchError(() => of({ ...cam, paused: false }))
          )
        );
        return forkJoin(pauseChecks);
      })
    );
  }

  getHealth(): Observable<any> {
    return this.api.get('health');
  }

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

  setPaused(cameraId: string, paused: boolean): Observable<{ camera_id: string; paused: boolean }> {
    return this.api.post(`api/cameras/${cameraId}/paused`, { paused });
  }
}
