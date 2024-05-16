# Installation
```sh
# curl -sfL https://get.k3s.io | sh -
# # or even better, so you can debug it later if it fails
# curl -sfL https://get.k3s.io > install.sh; sh install.sh
git clone https://aur.archlinux.org/k3s-bin
cd k3s-bin
makepkg -si

# Getting service status
systemctl start  k3s
systemctl status k3s

# Change cluster hostname
sudo vim /etc/rancher/k3s/k3s.yaml
systemctl restart k3s

# User's environment setup
mkdir ~/.kube
sudo cp /etc/rancher/k3s/k3s.yaml ~/.kube

# Detect control-plane and running pods
kubectl get nodes
kubectl -n kube-system get pods

```

### Tests
For testing purposes `test.yaml` working example is provided
```sh
kubectl create namespace test
kubectl -n test create configmap hello-world --from-file index.html
kubectl apply -f test.yaml
```
