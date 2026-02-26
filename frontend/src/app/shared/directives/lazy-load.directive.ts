import { Directive, ElementRef, Input, OnInit, inject } from '@angular/core';

/**
 * Directive for lazy loading images using Intersection Observer
 * Usage: <img [appLazyLoad]="imageUrl" alt="...">
 */
@Directive({
  selector: '[appLazyLoad]',
  standalone: true
})
export class LazyLoadDirective implements OnInit {
  @Input() appLazyLoad!: string;
  private el = inject(ElementRef);

  ngOnInit() {
    // Set placeholder background while loading
    this.el.nativeElement.style.backgroundColor = '#2a2a2a';

    const observer = new IntersectionObserver(
      (entries) => {
        entries.forEach(entry => {
          if (entry.isIntersecting) {
            const img = entry.target as HTMLImageElement;

            // Load image
            img.src = this.appLazyLoad;

            // Remove placeholder background on load
            img.onload = () => {
              img.style.backgroundColor = 'transparent';
            };

            // Handle errors
            img.onerror = () => {
              img.style.backgroundColor = '#4a4a4a';
              img.alt = 'Image failed to load';
            };

            // Stop observing after loading
            observer.unobserve(img);
          }
        });
      },
      {
        rootMargin: '50px' // Start loading 50px before entering viewport
      }
    );

    observer.observe(this.el.nativeElement);
  }
}
