package com.example.unix_omok;

import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;

@Entity
public class OmokData {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    private String idBlack;
    private String idWhite;
    private String winner;
    private Long playTime;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getIdBlack() {
        return idBlack;
    }

    public void setIdBlack(String idBlack) {
        this.idBlack = idBlack;
    }

    public String getIdWhite() {
        return idWhite;
    }

    public void setIdWhite(String idWhite) {
        this.idWhite = idWhite;
    }

    public String getWinner() {
        return winner;
    }

    public void setWinner(String winner) {
        this.winner = winner;
    }

    public Long getPlayTime() {
        return playTime;
    }

    public void setPlayTime(Long playTime) {
        this.playTime = playTime;
    }
}
