use std::sync::{Arc, Mutex, Weak};

use super::application::Application;

pub(super) struct Metadata {
    pub alive: bool,
    pub active: bool,
    pub locked: bool,
    pub owner: Weak<dyn BaseObject + Send + Sync>,
}

pub trait BaseObject {
    fn application(&self) -> Option<Arc<Mutex<Application>>>;
    fn on_create(&mut self) {}
    fn on_start(&mut self) {}
    fn on_update(&mut self) {}
    fn on_stop(&mut self) {}
    fn on_destroy(&mut self) {}

    fn on_active(&mut self) {}
    fn on_deactive(&mut self) {}
}

pub struct Object<T> {
    metadata: Metadata,
    application: Arc<Mutex<Application>>,
    inner: T,
}

impl<T> Object<T>
where
    T: BaseObject + 'static,
{
    pub fn active(mut self) -> Arc<Mutex<dyn BaseObject + 'static>> {
        if self.metadata.alive == true && self.metadata.active == false {
            self.metadata.active = true;
            self.inner.on_active();
            let locked = self.metadata.locked;

            let res = Arc::new(Mutex::new(self));
            if locked == false {
                res.lock()
                    .unwrap()
                    .application()
                    .unwrap()
                    .lock()
                    .unwrap()
                    .active_objects
                    .push_back(res.clone());
            }
            res
        } else {
            unreachable!()
        }
    }
}

impl<T> BaseObject for Object<T>
where
    T: BaseObject,
{
    fn application(&self) -> Option<Arc<Mutex<Application>>> {
        Some(self.application.clone())
    }
    fn on_create(&mut self) {
        self.inner.on_create()
    }
    fn on_start(&mut self) {
        self.inner.on_start()
    }
    fn on_update(&mut self) {
        self.inner.on_update()
    }
    fn on_stop(&mut self) {
        self.inner.on_stop()
    }
    fn on_destroy(&mut self) {
        self.inner.on_destroy()
    }

    fn on_active(&mut self) {
        self.inner.on_active()
    }
    fn on_deactive(&mut self) {
        self.inner.on_deactive()
    }
}
