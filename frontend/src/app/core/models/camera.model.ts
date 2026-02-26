export interface Camera {
  id: string;
  name: string;
  connected: boolean;
  buffer_size: number;
  last_event_time?: string;
}

export interface CameraStatus {
  cameras: Camera[];
}
