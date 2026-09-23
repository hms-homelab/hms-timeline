import { inject } from '@angular/core';
import { Router, Routes } from '@angular/router';
import { CameraViewComponent } from './features/camera-view/camera-view.component';

const DEFAULT_CAMERA = 'patio';

/**
 * Camera to open when the app loads at its root.
 *
 * Under HA ingress the app runs in an iframe that always loads the ingress root,
 * so a deep link such as /local_yolo_timeline/front_door never reaches the router.
 * The iframe is same-origin with the HA frontend, so the camera is read from the
 * second segment of the parent page's path instead. Anything else (standalone,
 * no camera segment, cross-origin) falls back to the default camera.
 */
export function initialCamera(): string {
  try {
    if (window.parent === window) return DEFAULT_CAMERA;
    const segments = window.parent.location.pathname.split('/').filter(Boolean);
    const camera = segments[1];
    return camera && /^[a-z0-9_]+$/.test(camera) ? camera : DEFAULT_CAMERA;
  } catch {
    return DEFAULT_CAMERA;
  }
}

export const routes: Routes = [
  {
    path: '',
    pathMatch: 'full',
    children: [],
    canActivate: [() => inject(Router).createUrlTree(['/', initialCamera()])]
  },
  {
    path: ':cameraId',
    component: CameraViewComponent,
    title: 'YOLO Detection'
  },
  {
    path: '**',
    redirectTo: '/patio'
  }
];
