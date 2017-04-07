#include "Particle.h"

Particle::Particle(float x, float y, float z)
{
	this->location = ofVec3f(x, y, z);
	this->velocity = ofVec3f(0, 0, 0);
	this->acceleration = ofVec3f(0, 0, 0);

	this->max_speed = 30;
	this->max_force = 5;
	this->radius = 30;
	this->angle = ofRandom(360);

	float body_value = ofRandom(255);
	this->body_color.setHsb(body_value, 255, 255, 255);
	this->body_size = 5;
}

Particle::~Particle()
{

}

void Particle::applyForce(ofVec3f force)
{
	this->acceleration += force;
}

ofVec3f Particle::seek(ofVec3f target)
{
	ofVec3f desired = this->location - target;
	float distance = desired.length();
	desired.normalize();
	if (distance < this->radius)
	{
		float m = ofMap(distance, 0, this->radius, 0, this->max_speed);
		desired *= m;
	}
	else
	{
		desired *= this->max_speed;
	}

	ofVec3f steer = this->velocity - desired;
	steer.limit(this->max_force);
	return steer;
}

ofVec3f Particle::separate(vector<Particle> particles)
{
	float desiredseparation = this->body_size * 10.0;
	ofVec3f sum;
	int count = 0;
	for (auto& p : particles) {
		float distance = this->location.distance(p.location);
		if (distance > 0 && distance < desiredseparation) {
			ofVec3f diff = this->location - p.location;
			diff.normalize();
			diff /= distance;
			sum += diff;
			count++;
		}
	}

	if (count > 0) {
		sum /= count;
		sum.limit(this->max_speed);
		ofVec3f steer = this->velocity - sum;
		steer.limit(this->max_force);

		return steer;
	}

	return ofVec3f(0, 0, 0);
}

ofVec3f Particle::align(vector<Particle> particles)
{
	float neighbordist = this->body_size * 20;
	ofVec3f sum;
	int count = 0;
	for (auto& p : particles) {
		float distance = this->location.distance(p.location);
		if (distance > 0 && distance < neighbordist) {
			sum += p.velocity;
			count++;
		}
	}

	if (count > 0) {
		sum /= count;
		sum.normalize();
		sum.limit(this->max_speed);
		ofVec3f steer = this->velocity - sum;
		steer.limit(this->max_force);

		return steer;
	}

	return ofVec3f(0, 0, 0);
}

ofVec3f Particle::cohesion(vector<Particle> particles)
{
	float neighbordist = this->body_size * 20;
	ofVec3f sum;
	int count = 0;
	for (auto& p : particles) {
		float distance = this->location.distance(p.location);
		if (distance > 0 && distance < neighbordist) {
			sum += p.location;
			count++;
		}
	}

	if (count > 0) {
		sum /= count;
		return seek(sum);
	}

	return ofVec3f(0, 0, 0);
}

void Particle::flok(vector<Particle> particles)
{
	ofVec3f sep = this->separate(particles);
	ofVec3f ali = this->align(particles);
	ofVec3f coh = this->cohesion(particles);

	sep *= 1.5;
	ali *= 1.0;
	coh *= 1.0;

	this->applyForce(sep);
	this->applyForce(ali);
	this->applyForce(coh);
}

void Particle::think()
{
	ofVec3f future;
	future.set(this->velocity);
	future.normalize();
	future *= 50;
	future += this->location;

	ofVec3f random = ofVec3f(ofRandom(-ofGetWidth() / 2, ofGetWidth() / 2), ofRandom(-ofGetHeight() / 2, ofGetHeight() / 2), ofRandom(-512, 512));
	random.normalize();
	random *= 50;
	random += future;

	this->applyForce(this->seek(random));
}

void Particle::update()
{
	this->think();

	this->velocity += this->acceleration;
	this->velocity.limit(max_speed);
	this->location += this->velocity;
	
	this->acceleration *= 0;
	this->velocity *= 0.98;

	this->log.push_back(this->location);
	if (this->log.size() > 15) {
		this->log.erase(this->log.begin());
	}
}

void Particle::draw()
{
	ofSetColor(this->body_color);
	ofPushMatrix();
	ofTranslate(this->location);
	ofSphere(this->body_size);
	ofPopMatrix();

	int span = 0;
	if (log.size() != 0) { span = 255 / log.size(); }
	for (int i = this->log.size() - 1; i > 0; i--)
	{
		ofSetColor(this->body_color, span * i);
		ofLine(this->log[i], this->log[i - 1]);
	}
	
}

void Particle::borders()
{
	ofVec3f distance = this->location - ofVec3f(0, 0, 0);
	if(distance.length() > 300) {
		distance.normalize();
		distance *= -this->max_force;
		applyForce(distance);
	}
}