use std::{
    collections::LinkedList,
    sync::{Arc, Mutex},
};

use super::base_object::{BaseObject, Metadata};

pub struct Application {
    pub(super) active_objects: LinkedList<Arc<Mutex<dyn BaseObject>>>,
    pub(super) passive_objects: LinkedList<Arc<Mutex<dyn BaseObject>>>,
    metadata: Metadata,
    graphics: Arc<()>,
}

impl Application {
    fn create_graphics(&mut self) {}
    fn create_launcher_scene(&mut self) {}
    fn create_window(&self) {}
    fn launch_scene(&mut self) {}
    fn terminate_scene(&self) {}
}

impl BaseObject for Application {
    fn application(&self) -> Option<Arc<Mutex<Application>>> {
        None
    }
}
