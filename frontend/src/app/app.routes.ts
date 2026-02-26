import { Routes } from '@angular/router';
import { CameraViewComponent } from './features/camera-view/camera-view.component';

export const routes: Routes = [
  {
    path: '',
    redirectTo: 'patio',
    pathMatch: 'full'
  },
  {
    path: ':cameraId',
    component: CameraViewComponent,
    title: 'YOLO Detection'
  },
  {
    path: '**',
    redirectTo: 'patio'
  }
];
