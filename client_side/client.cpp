#include "client.h"

Client::Client()
{
    setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(receivedData()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSocket(QAbstractSocket::SocketError)));

    messageLength = 0;
}

// Tentative de connexion au serveur
void Client::on_connectionButton_clicked()
{
    // On annonce sur la fenêtre qu'on est en train de se connecter
    messagesList->append(tr("<em>Tentative de connexion en cours...</em>"));
    connectionButton->setEnabled(false);

    socket->abort(); // On désactive les connexions précédentes s'il y en a
    socket->connectToHost(serverIP->text(), serverPort->value()); // On se connecte au serveur demandé
}

// Envoi d'un message au serveur
void Client::on_sendButton_clicked()
{
    QByteArray package;
    QDataStream out(&package, QIODevice::WriteOnly);

    // On prépare le paquet à envoyer
    QString sendMessage = tr("<strong>") + pseudo->text() +tr("</strong> : ") + message->text();

    out << (quint16) 0;
    out << sendMessage;
    out.device()->seek(0);
    out << (quint16) (package.size() - sizeof(quint16));

    socket->write(package); // On envoie le paquet

    message->clear(); // On vide la zone d'écriture du message
    message->setFocus(); // Et on remet le curseur à l'intérieur
}

// Appuyer sur la touche Entrée a le même effet que cliquer sur le bouton "Envoyer"
void Client::on_message_returnPressed()
{
    on_sendButton_clicked();
}

// On a reçu un paquet (ou un sous-paquet)
void Client::receivedData()
{
    /* Même principe que lorsque le serveur reçoit un paquet :
    On essaie de récupérer la taille du message
    Une fois qu'on l'a, on attend d'avoir reçu le message entier (en se basant sur la taille annoncée tailleMessage)
    */
    QDataStream in(socket);

    if (messageLength == 0)
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
             return;

        in >> messageLength;
    }

    if (socket->bytesAvailable() < messageLength)
        return;


    // Si on arrive jusqu'à cette ligne, on peut récupérer le message entier
    QString messageReceived;
    in >> messageReceived;

    // On affiche le message sur la zone de Chat
    messagesList->append(messageReceived);

    // On remet la taille du message à 0 pour pouvoir recevoir de futurs messages
    messageLength = 0;
}

// Ce slot est appelé lorsque la connexion au serveur a réussi
void Client::connected()
{
    messagesList->append(tr("<em>Connexion réussie !</em>"));
    connectionButton->setEnabled(true);
}

// Ce slot est appelé lorsqu'on est déconnecté du serveur
void Client::disconnected()
{
    messagesList->append(tr("<em>Déconnecté du serveur</em>"));
}

// Ce slot est appelé lorsqu'il y a une erreur
void Client::socketError(QAbstractSocket::SocketError error)
{
    switch(error) // On affiche un message différent selon l'erreur qu'on nous indique
    {
        case QAbstractSocket::HostNotFoundError:
            messagesList->append(tr("<em>ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.</em>"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            messagesList->append(tr("<em>ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.</em>"));
            break;
        case QAbstractSocket::RemoteHostClosedError:
            messagesList->append(tr("<em>ERREUR : le serveur a coupé la connexion.</em>"));
            break;
        default:
            messagesList->append(tr("<em>ERREUR : ") + socket->errorString() + tr("</em>"));
    }

    connectionButton->setEnabled(true);
}
